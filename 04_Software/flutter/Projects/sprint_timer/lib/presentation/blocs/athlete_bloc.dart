import 'package:bloc/bloc.dart';
import 'package:equatable/equatable.dart';
import '../../domain/entities/entities.dart';
import '../../domain/repositories/repositories.dart';

// --- Events ---
abstract class AthleteEvent extends Equatable {
  const AthleteEvent();
  @override
  List<Object?> get props => [];
}

class LoadAthletes extends AthleteEvent {}

class CreateAthlete extends AthleteEvent {
  final Athlete athlete;
  const CreateAthlete(this.athlete);
  @override List<Object?> get props => [athlete];
}

class UpdateAthlete extends AthleteEvent {
  final Athlete athlete;
  const UpdateAthlete(this.athlete);
  @override List<Object?> get props => [athlete];
}

class DeleteAthlete extends AthleteEvent {
  final int athleteId;
  const DeleteAthlete(this.athleteId);
  @override List<Object?> get props => [athleteId];
}

// --- States ---
abstract class AthleteState extends Equatable {
  const AthleteState();
  @override
  List<Object?> get props => [];
}

class AthletesInitial extends AthleteState {}
class AthletesLoading extends AthleteState {}
class AthletesLoaded extends AthleteState {
  final List<Athlete> athletes;
  const AthletesLoaded(this.athletes);
  @override
  List<Object?> get props => [athletes];
}
class AthletesError extends AthleteState {
  final String message;
  const AthletesError(this.message);
  @override
  List<Object?> get props => [message];
}

// --- BLoC ---
class AthleteBloc extends Bloc<AthleteEvent, AthleteState> {
  final AthleteRepository _repository;

  AthleteBloc(this._repository) : super(AthletesInitial()) {
    on<LoadAthletes>(_onLoad);
    on<CreateAthlete>(_onCreate);
    on<UpdateAthlete>(_onUpdate);
    on<DeleteAthlete>(_onDelete);
  }

  Future<void> _onLoad(LoadAthletes event, Emitter<AthleteState> emit) async {
    emit(AthletesLoading());
    try {
      final athletes = await _repository.getAll();
      emit(AthletesLoaded(athletes));
    } catch (e) {
      emit(AthletesError('Error loading athletes: $e'));
    }
  }

  Future<void> _onCreate(CreateAthlete event, Emitter<AthleteState> emit) async {
    try {
      await _repository.save(event.athlete);
      add(LoadAthletes());
    } catch (e) {
      emit(AthletesError('Error creating athlete: $e'));
    }
  }

  Future<void> _onUpdate(UpdateAthlete event, Emitter<AthleteState> emit) async {
    try {
      await _repository.save(event.athlete);
      add(LoadAthletes());
    } catch (e) {
      emit(AthletesError('Error updating athlete: $e'));
    }
  }

  Future<void> _onDelete(DeleteAthlete event, Emitter<AthleteState> emit) async {
    try {
      await _repository.delete(event.athleteId);
      add(LoadAthletes());
    } catch (e) {
      emit(AthletesError('Error deleting athlete: $e'));
    }
  }
}
